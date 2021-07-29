(: insert-start :)
declare variable $input-context1 external;
declare variable $input-context2 external;
(: insert-end :)

<result>
  {
    for $i in $input-context1//item_tuple
    let $b := $input-context2//bid_tuple[itemno = $i/itemno]
    where contains(exactly-one($i/description), "Bicycle")
    order by $i/itemno
    return
        <item_tuple>
            { $i/itemno }
            { $i/description }
            <high_bid>{ max($b/bid) }</high_bid>
        </item_tuple>
  }
</result> 
