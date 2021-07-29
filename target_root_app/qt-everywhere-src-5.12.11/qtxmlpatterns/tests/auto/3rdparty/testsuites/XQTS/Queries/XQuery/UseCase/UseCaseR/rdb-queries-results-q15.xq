(: insert-start :)
declare variable $input-context1 external;
declare variable $input-context2 external;
(: insert-end :)

<result>
  {
    for $u in $input-context1//user_tuple
    let $b := $input-context2//bid_tuple[userid=$u/userid and bid>=100]
    where count($b) > 1
    return
        <big_spender>{ $u/name/text() }</big_spender>
  }
</result>