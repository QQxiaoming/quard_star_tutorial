(: insert-start :)
declare variable $input-context1 external;
declare variable $input-context2 external;
declare variable $input-context3 external;
(: insert-end :)
<result>
  {
   unordered (
    for $seller in $input-context2//user_tuple,
        $buyer in $input-context2//user_tuple,
        $item in $input-context1//item_tuple,
        $highbid in  $input-context3//bid_tuple
    where $seller/name = "Tom Jones"
      and $seller/userid  = $item/offered_by
      and contains(exactly-one($item/description), "Bicycle")
      and $item/itemno  = $highbid/itemno
      and $highbid/userid  = $buyer/userid
      and $highbid/bid = max(
                              $input-context3//bid_tuple
                                [itemno = $item/itemno]/bid
                         )
    return
        <jones_bike>
            { $item/itemno }
            { $item/description }
            <high_bid>{ $highbid/bid }</high_bid>
            <high_bidder>{ $buyer/name }</high_bidder>
        </jones_bike>
    )
  }
</result> 
