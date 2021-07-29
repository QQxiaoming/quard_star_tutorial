(: insert-start :)
declare variable $input-context1 external;
declare variable $input-context2 external;
(: insert-end :)

<result>
 {
    for $uid in distinct-values($input-context1//userid),
        $u in $input-context2//user_tuple[userid = $uid]
    let $b := $input-context1//bid_tuple[userid = $uid]
    order by exactly-one($u/userid)
    return
        <bidder>
            { $u/userid }
            { $u/name }
            <bidcount>{ count($b) }</bidcount>
            <avgbid>{ avg($b/bid) }</avgbid>
        </bidder>
  }
</result> 
