<?xml version="1.0" encoding="UTF-8"?>
<figlist>
  {
    for $f in doc("book.xml")//figure
    return
        <figure>
            { $f/@* }
            { $f/title }
        </figure>
  }
</figlist> 
