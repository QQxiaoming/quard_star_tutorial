<?xml version="1.0" encoding="UTF-8"?>
declare function local:toc($book-or-section as element()) as element()*
{
    for $section in $book-or-section/section
    return
      <section>
         { $section/@* , $section/title , local:toc($section) }			
      </section>
};

<toc>
   {
     for $s in doc("book.xml")/book return local:toc($s)
   }
</toc> 
