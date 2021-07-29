(: insert-start :)
declare construction strip;
declare variable $input-context1 external;
declare variable $input-context2 external;
(: insert-end :)

declare function local:bid_summary()
  as element()*
{
    for $i in distinct-values($input-context2//itemno)
    let $b := $input-context2//bid_tuple[itemno = $i]
    return
        <bid_count>
            <itemno>{ $i }</itemno>
            <nbids>{ count($b) }</nbids>
        </bid_count>
};

<result>
 {
    let $bid_counts := local:bid_summary(),
        $maxbids := max($bid_counts/nbids),
        $maxitemnos := $bid_counts[nbids = $maxbids]
    for $item in $input-context1//item_tuple,
            $bc in $bid_counts
    where $bc/nbids =  $maxbids and $item/itemno = $bc/itemno
    return
        <popular_item>
            { $item/itemno }
            { $item/description }
            <bid_count>{ $bc/nbids/text() }</bid_count>
        </popular_item>
 }
</result> 
