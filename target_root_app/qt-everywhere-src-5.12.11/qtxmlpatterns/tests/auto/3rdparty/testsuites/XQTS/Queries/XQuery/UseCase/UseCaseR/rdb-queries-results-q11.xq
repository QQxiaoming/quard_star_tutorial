(: insert-start :)
declare variable $input-context1 external;
declare variable $input-context2 external;
(: insert-end :)

let $highbid := max($input-context2//bid_tuple/bid)
return
    <result>
     {
        for $item in $input-context1//item_tuple,
            $b in $input-context2//bid_tuple[itemno = $item/itemno]
        where $b/bid = $highbid
        return
            <expensive_item>
                { $item/itemno }
                { $item/description }
                <high_bid>{ $highbid }</high_bid>
            </expensive_item>
     }
    </result> 