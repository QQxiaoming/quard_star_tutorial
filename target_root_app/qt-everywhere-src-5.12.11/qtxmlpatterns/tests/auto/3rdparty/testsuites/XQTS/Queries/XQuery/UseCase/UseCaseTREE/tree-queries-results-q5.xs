<?xml version="1.0" encoding="UTF-8"?>
<section_list>
  {
    for $s in doc("book.xml")//section
    let $f := $s/figure
    return
        <section title="{ $s/title/text() }" figcount="{ count($f) }"/>
  }
</section_list> 
