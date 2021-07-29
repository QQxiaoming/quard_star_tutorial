(: insert-start :)
declare variable $input-context1 external;
declare variable $input-context2 external;
declare variable $input-context3 external;
(: insert-end :)

<frequent_bidder>
  {
    for $u in $input-context2//user_tuple
    where 
      every $item in $input-context1//item_tuple satisfies 
        some $b in $input-context3//bid_tuple satisfies 
          ($item/itemno = $b/itemno and $u/userid = $b/userid)
    return
        $u/name
  }
</frequent_bidder>