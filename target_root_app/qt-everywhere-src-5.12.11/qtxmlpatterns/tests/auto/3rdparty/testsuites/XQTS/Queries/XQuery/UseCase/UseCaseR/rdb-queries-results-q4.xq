(: insert-start :)
declare variable $input-context1 external;
declare variable $input-context2 external;
(: insert-end :)

<result>
  {
    for $i in $input-context1//item_tuple
    where empty ($input-context2//bid_tuple[itemno = $i/itemno])
    return
        <no_bid_item>
            { $i/itemno }
            { $i/description }
        </no_bid_item>
  }
</result>