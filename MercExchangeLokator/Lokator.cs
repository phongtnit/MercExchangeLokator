using System;
using System.Collections.Generic;
using System.Drawing;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace MercExchangeLokator
{

    public class MercExchangeFoundArguments : EventArgs
    {
        public System.Drawing.Rectangle Location { get; private set; }
        public SizeF ScaleFactor { get; private set; }  

        public MercExchangeFoundArguments(System.Drawing.Rectangle location, SizeF scaleFactor)
        {
            Location = location;
            ScaleFactor = scaleFactor;
        }
    }

    internal class Lokator
    {
        public event EventHandler<MercExchangeFoundArguments> onMercExchangeFound;
        public event EventHandler<MercExchangeFoundArguments> onMercExchangeNotFound;
        public void Found(System.Drawing.Rectangle location, SizeF scaleFactor) 
        {
            onMercExchangeFound?.Invoke(null, new MercExchangeFoundArguments(location, scaleFactor));    
        }
        public void NotFound()
        {
            onMercExchangeNotFound(null, new MercExchangeFoundArguments(System.Drawing.Rectangle.Empty, new SizeF(0,0)));
        }
    }
}
