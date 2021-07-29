(: insert-start :)
declare variable $input-context external;
(: insert-end :)

<critical_sequence>
 {
  let $proc := $input-context//section[section.title="Procedure"][1],
      $i1 :=  ($proc//incision)[1],
      $i2 :=  ($proc//incision)[2]
  for $n in $proc//node() except $i1//node()
  where $n >> $i1 and $n << $i2
  return $n 
 }
</critical_sequence>
