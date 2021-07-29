(: insert-start :)
declare variable $input-context external;
(: insert-end :)

<Q1>
  {
    for $n in distinct-values(
                  for $i in ($input-context//* | $input-context//@*)
                  return namespace-uri($i) 
               )
    return  <ns>{$n}</ns>
  }
</Q1> 