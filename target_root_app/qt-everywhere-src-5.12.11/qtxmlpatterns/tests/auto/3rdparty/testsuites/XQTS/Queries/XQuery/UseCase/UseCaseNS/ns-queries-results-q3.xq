declare namespace dt = "http://www.w3.org/2001/XMLSchema";
(: insert-start :)
declare variable $input-context external;
(: insert-end :)
<Q3>
  {
    $input-context//*[@dt:*]
  }
</Q3> 