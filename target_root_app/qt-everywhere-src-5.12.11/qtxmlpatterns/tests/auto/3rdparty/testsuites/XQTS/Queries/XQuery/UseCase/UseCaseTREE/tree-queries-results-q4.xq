(: insert-start :)
declare variable $input-context external;
(: insert-end :)

<top_section_count>
 { 
   count($input-context/book/section) 
 }
</top_section_count>