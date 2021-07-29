(: insert-start :)
declare variable $input-context1 external;
declare variable $input-context2 external;
(: insert-end :)

let $allbikes := $input-context1//item_tuple
                    [contains(exactly-one(description), "Bicycle") 
                     or contains(exactly-one(description), "Tricycle")]
let $bikebids := $input-context2//bid_tuple[itemno = $allbikes/itemno]
return
    <high_bid>
      { 
        max($bikebids/bid) 
      }
    </high_bid> 
