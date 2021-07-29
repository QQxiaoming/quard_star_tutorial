(: Name: errors-and-optimization-6.xq :)
(: Description: 

   Expressions must not be rewritten in such a way as to create or
   remove static errors. 
   
:)

(: insert-start :)
(: insert-end :)


if (true()) 
  then 1 
  else 
    let $unbound:var := 2 
    return $unbound:var