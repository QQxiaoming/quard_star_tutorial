(: insert-start :)
declare variable $input-context1 external;
declare variable $input-context2 external;
(: insert-end :)

<result>
 {
    for $highbid in $input-context1//bid_tuple,
        $user in $input-context2//user_tuple
    where $user/userid = $highbid/userid 
      and $highbid/bid = max($input-context1//bid_tuple[itemno=$highbid/itemno]/bid)
    order by exactly-one($highbid/itemno)
    return
        <high_bid>
            { $highbid/itemno }
            { $highbid/bid }
            <bidder>{ $user/name/text() }</bidder>
        </high_bid>
  }
</result> 
