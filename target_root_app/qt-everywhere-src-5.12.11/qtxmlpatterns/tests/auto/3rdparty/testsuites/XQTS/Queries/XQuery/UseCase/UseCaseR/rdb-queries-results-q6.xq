(: insert-start :)
declare variable $input-context1 external;
declare variable $input-context2 external;
(: insert-end :)

<result>
  {
    for $item in $input-context1//item_tuple
    let $b := $input-context2//bid_tuple[itemno = $item/itemno]
    let $z := max($b/bid)
    where exactly-one($item/reserve_price) * 2 < $z
    return
        <successful_item>
            { $item/itemno }
            { $item/description }
            { $item/reserve_price }
            <high_bid>{$z }</high_bid>
         </successful_item>
  }
</result> 
