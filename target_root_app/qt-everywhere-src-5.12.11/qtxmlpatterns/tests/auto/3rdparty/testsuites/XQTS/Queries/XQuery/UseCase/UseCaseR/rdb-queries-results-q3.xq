(: insert-start :)
declare variable $input-context1 external;
declare variable $input-context2 external;
(: insert-end :)
<result>
  {
    for $u in $input-context2//user_tuple
    for $i in $input-context1//item_tuple
    where $u/rating > "C" 
       and $i/reserve_price > 1000 
       and $i/offered_by = $u/userid
    return
        <warning>
            { $u/name }
            { $u/rating }
            { $i/description }
            { $i/reserve_price }
        </warning>
  }
</result>