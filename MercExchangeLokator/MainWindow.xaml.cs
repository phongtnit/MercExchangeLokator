using System;
using System.Collections.Generic;
using System.Drawing;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows;

using System.Windows.Controls;
using System.Windows.Data;
using System.Windows.Documents;
using System.Windows.Input;
using System.Windows.Media;
using System.Windows.Media.Imaging;
using System.Windows.Navigation;
using System.Windows.Shapes;

using Emgu;
using Emgu.CV;
using Emgu.CV.CvEnum;
using Emgu.CV.Structure;

using com.HellScape.ScreenCapture;
using System.Reflection;

namespace MercExchangeLokator
{
    /// <summary>
    /// Interaction logic for MainWindow.xaml
    /// </summary>
    /// 
  
    
    public partial class MainWindow : Window
    {
        //--- grab screen fast.
        //--- check for merchant exchange.
        //--- found it, render rectangle.
        bool bCaptureStopping = false;
        bool bCapturing = false;
        private object bmpLocker = new object();

        Image<Bgr, byte> refImage = null;
        Image<Bgr, byte> scaledRefImage = null;
        System.Drawing.Size originalImageSize = System.Drawing.Size.Empty;
        System.Drawing.Size scaledImageSize = System.Drawing.Size.Empty;
        System.Threading.Thread capturingThread;

        RenderCanvas RenderCanvas = null;

        bool useOptimizedMethod = true;

        public string basePath  => System.AppDomain.CurrentDomain.BaseDirectory;
        public string refFile => $@"{basePath}Images\Ref\merc_exchange_sample02.png";
        string debugSaveOutputPath => $@"{basePath}Tests\";
        
        private double defaultScaleFactor = 1.0f;

        Lokator Lokator { get; set; }

        double matchingTolerence = 0.6;

        public double ScaleFactor
        {
            get { return defaultScaleFactor; }
            set { defaultScaleFactor = value; }
        }

        public MainWindow()
        {
            InitializeComponent();
            Lokator = new Lokator();
            Lokator.onMercExchangeFound += Lokator_onMercExchangeFound;
            Lokator.onMercExchangeNotFound += Lokator_onMercExchangeNotFound;
        }

        private void Lokator_onMercExchangeNotFound(object sender, MercExchangeFoundArguments e)
        {
            this.Dispatcher.BeginInvoke(new Action(() =>
            {
               RenderCanvas.Canvas01.Children.Clear();
            }));
        }

        bool isRendering = false;
        private void Lokator_onMercExchangeFound(object sender, MercExchangeFoundArguments e)
        {

            var match = e.Location;
            
            var dpiXProperty = typeof(SystemParameters).GetProperty("DpiX", BindingFlags.NonPublic | BindingFlags.Static);
            var dpiYProperty = typeof(SystemParameters).GetProperty("Dpi", BindingFlags.NonPublic | BindingFlags.Static);

            var dpiX = (int)dpiXProperty.GetValue(null, null) / 96.0;
            var dpiY = (int)dpiYProperty.GetValue(null, null) / 96.0;

            if (useOptimizedMethod)
            {
                double mx, my = 0.0f;
                mx = ((double)match.X * e.ScaleFactor.Width);
                my = ((double)match.Y * e.ScaleFactor.Height);
                match.X = (int)Math.Round(mx);
                match.Y = (int)Math.Round(my);  
            }

            RenderCanvas.Dispatcher.BeginInvoke(new Action(() =>
            {
                if (!bCaptureStopping)
                {
                    if (!isRendering)
                    {
                        isRendering = true;
                        var height = RenderCanvas.Canvas01.ActualHeight * dpiY;
                        var width = RenderCanvas.Canvas01.ActualWidth * dpiX;

                        WriteableBitmap wb = BitmapFactory.New((int)width, (int)height);

                        using (wb.GetBitmapContext())
                        {
                            var x = match.X / dpiX - 42;
                            var y = match.Y / dpiY - 8;
                            var x2 = x + 128;
                            var y2 = y + 128;
                            var thickness = 10;

                            wb.DrawRectangle((int)x, (int)y, (int)x2, (int)y2, System.Windows.Media.Colors.Red);
                            for (var i = 0; i < thickness; i++)
                            {
                                wb.DrawRectangle((int)x--, (int)y--, (int)x2++, (int)y2++, System.Windows.Media.Colors.Red);
                            }

                            System.Windows.Controls.Image image = new System.Windows.Controls.Image();
                            image.Source = wb;

                            if (RenderCanvas.Canvas01.Children.Count > 1)
                            {
                                RenderCanvas.Canvas01.Children.RemoveRange(RenderCanvas.Canvas01.Children.Count, 2);
                            }
                            else
                                RenderCanvas.Canvas01.Children.Add(image);

                            isRendering = false;
                        }
                    }
                }
            }));

        }

        

        #region Screen Capturing Functions
        private void performTemplateMatching(Bitmap bitmap, double threshold)
        {

            if (bitmap == null)
                return;

            Image<Bgr, byte> src = bitmap.ToImage<Bgr, byte>();

            while (true)
            {
                double[] minValues, maxValues;
                System.Drawing.Point[] minLocations, maxLocations;

                using (Image<Gray, float> resultImage = src.MatchTemplate(useOptimizedMethod == true ? scaledRefImage : refImage, TemplateMatchingType.CcoeffNormed))
                {

                    //CvInvoke.Threshold(resultImage, resultImage, threshold, 1, ThresholdType.ToZero);
                    resultImage.MinMax(out minValues, out maxValues, out minLocations, out maxLocations);

                    if (maxValues[0] > threshold)
                    {
                        double scaleFactorX = (double)(originalImageSize.Width / scaledImageSize.Width);
                        double scaleFactorY = (double)(originalImageSize.Height / scaledImageSize.Height);
                        Lokator.Found(new System.Drawing.Rectangle(
                            new System.Drawing.Point(maxLocations[0].X, maxLocations[0].Y - 32),
                            new System.Drawing.Size(128, 128)),
                            new System.Drawing.SizeF((float)scaleFactorX, (float)scaleFactorY));
                        break;
                    }
                    else
                    {
                        Lokator.NotFound();
                        break;
                    }
                }
            }
            src.Dispose();
            src = null;
        }

        private Task performTemplateMatchingAsync(Bitmap bitmap, double threshold)
        {
            return Task.Run(() => { performTemplateMatching(bitmap, threshold); } );
        }

        private Bitmap ResizeByHalf(Image<Bgr, byte> image)
        {
            image.Resize(image.Width / 2, image.Height / 2, Inter.Linear);  
            return image.ToBitmap<Bgr, byte>(); 
        }
        private async void CapturingThread(Bitmap bitmap)
        {
            //-- process bitmap
            Bitmap bclone = (Bitmap)bitmap.Clone();
            await performTemplateMatchingAsync(bclone, matchingTolerence);
            bclone.Dispose();
        }
        public void StartCapturing()
        {
            Snapture.onFrameCaptured += Snapture_onFrameCaptured;
            Snapture.FPS = 30;
            //-- DX is causing memory leaks and eating memory.
            Snapture.Start(FrameCapturingMethod.GDI);

            //-- now everything is completely manual when capturing. CLI C++ doesn't do any while loop.
            int sh = Snapture.ScreenHeight;
            int sw = Snapture.ScreenWidth;
            int x = sw / 2; //-- 1920 
            int y = sh / 2; //-- 1080

            int left = (int)Math.Round(x * 0.7d);
            int top = (int)Math.Round(y * 0.6157);
            int width = (int)Math.Round(x * 0.7906);
            int height = (int)Math.Round(y * 0.6975);
            
            Snapture.CaptureRegion(left,top, width, height);  

        }

        private void Snapture_onFrameCaptured(object sender, FrameCapturedEventArgs e)
        {
            if (bCapturing)
            {
                if (e.ScreenCapturedBitmap == null)
                    return;
                else
                {
                    originalImageSize = e.ScreenCapturedBitmap.Size;
                    //e.ScreenCapturedBitmap.Save($"{basePath}Screenshots/ScreenCaptured{e.FrameCount}.jpg");
                    //-- resize screen captured here.
                    if (useOptimizedMethod)
                    {
                        Image<Bgr, byte> resizedImage = e.ScreenCapturedBitmap.ToImage<Bgr, byte>();
                        Bitmap resizedBitmap = resizedImage.Resize(ScaleFactor, Inter.Linear).ToBitmap();
                        scaledImageSize = resizedBitmap.Size;

                        CapturingThread(resizedBitmap);
                        resizedBitmap.Dispose();
                        resizedImage.Dispose();
                    }
                    else
                    {
                        CapturingThread(e.ScreenCapturedBitmap);
                    }
                    e.ScreenCapturedBitmap.Dispose();
                }
            }
        }

        #endregion

        #region Main Functions
        protected override void OnClosed(EventArgs e)
        {
            base.OnClosed(e);
            if (Snapture.isActive)
                Snapture.Stop();

            Environment.Exit(1);
        }
        private void Window_Closing(object sender, System.ComponentModel.CancelEventArgs e)
        {
        }

        private void Window_Loaded(object sender, RoutedEventArgs e)
        {
            this.Topmost = true;
          
            refImage = new Image<Bgr, byte>(refFile);
            if (refImage != null)
                System.Diagnostics.Debug.WriteLine($"Successfully loaded reference image: {refFile}");

            if(useOptimizedMethod)
            {
                ScaleFactor = 0.5f;

                scaledRefImage = refImage.Resize(ScaleFactor, Inter.Linear);
            }
            RenderCanvas = new RenderCanvas();
            RenderCanvas.Topmost = true;
            RenderCanvas.Show();
        }

        private void Window_Unloaded(object sender, RoutedEventArgs e)
        {

        }

        private void StartCaptureBtn_MouseDown(object sender, MouseButtonEventArgs e)
        {

            this.Dispatcher.BeginInvoke(new Action(() =>
            {
                StartCaptureBtn.IsEnabled = false;
                StopCaptureBtn.IsEnabled = true;
                bCapturing = true;
                bCaptureStopping = false;

                Task CapturingTask = new Task(() =>
                {
                    StartCapturing();
                });
                CapturingTask.Start();
                /*
                if (capturingThread == null)
                {
                    capturingThread = new System.Threading.Thread(new System.Threading.ThreadStart(StartCapturing));
                    capturingThread.Name = "Screen Capturing Thread";
                    capturingThread.Start();
                }
                else {
                }
                */

            }
            ));
        }

        private void StopCaptureBtn_MouseDown(object sender, MouseButtonEventArgs e)
        {
            this.Dispatcher.BeginInvoke(new Action(() =>
            {
                if (bCapturing)
                {
                    Snapture.Stop();
                    bCaptureStopping = true;
                    bCapturing = false;
                    this.Dispatcher.BeginInvoke(new Action(() =>
                    {
                        RenderCanvas.Canvas01.Children.Clear();
                    }));

                    StartCaptureBtn.IsEnabled = true;
                    StopCaptureBtn.IsEnabled = false;

                    if (capturingThread != null)
                    {
                        capturingThread.Abort();
                        capturingThread = null;
                    }

                    System.Diagnostics.Debug.WriteLine($"Screen Capturing stopped.");
                }

            }));
        }

        private void SettingsBtn_MouseDown(object sender, MouseButtonEventArgs e)
        {

        }

        #endregion
    }
}
