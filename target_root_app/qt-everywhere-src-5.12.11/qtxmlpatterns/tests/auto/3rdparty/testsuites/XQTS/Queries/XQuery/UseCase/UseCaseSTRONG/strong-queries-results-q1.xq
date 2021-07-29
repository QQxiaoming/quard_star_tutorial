import schema namespace ipo = "http://www.example.com/IPO";

(: insert-start :)
declare variable $input-context external;
(: insert-end :)


                        
count( 
  $input-context//shipTo[. instance of element(*, ipo:UKAddress)]
)
