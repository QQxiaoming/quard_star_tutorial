declare namespace ma = "http://www.example.com/AuctionWatch";
declare namespace anyzone = "http://www.example.com/auctioneers#anyzone";
(: insert-start :)
declare variable $input-context external;
(: insert-end :)

<Q6 xmlns:ma="http://www.example.com/AuctionWatch">
  {
    $input-context//ma:Auction[@anyzone:ID]/ma:Schedule/ma:Close
  }
</Q6>