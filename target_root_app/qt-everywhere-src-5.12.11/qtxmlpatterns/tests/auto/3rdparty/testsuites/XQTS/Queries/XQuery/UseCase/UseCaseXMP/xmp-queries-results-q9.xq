(: insert-start :)
declare variable $input-context external;
(: insert-end :)

<results>
  {
    for $t in $input-context//(chapter | section)/title
    where contains(exactly-one($t/text()), "XML")
    return $t
  }
</results> 
