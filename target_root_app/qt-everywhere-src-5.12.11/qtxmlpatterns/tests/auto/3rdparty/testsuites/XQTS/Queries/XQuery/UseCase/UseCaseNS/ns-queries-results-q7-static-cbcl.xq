declare namespace ma = "http://www.example.com/AuctionWatch";
(: insert-start :)
declare variable $input-context external;
(: insert-end :)

<Q7 xmlns:xlink="http://www.w3.org/1999/xlink">
  {
    for $a in $input-context//ma:Auction
    let $seller_id := $a/ma:Trading_Partners/ma:Seller/*:ID,
        $buyer_id := $a/ma:Trading_Partners/ma:High_Bidder/*:ID
    where namespace-uri(exactly-one($seller_id)) = namespace-uri(exactly-one($buyer_id))
    return
        $a/ma:AuctionHomepage
  }
</Q7> 
