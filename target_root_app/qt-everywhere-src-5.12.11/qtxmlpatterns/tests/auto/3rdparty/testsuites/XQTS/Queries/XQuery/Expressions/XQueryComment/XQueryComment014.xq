(: Name: XQueryComment014 :)
(: Description: Comments inside a cast expression :)

(: insert-start :)
declare variable $input-context external;
(: insert-end :)

"10" cast as (: type comment :) xs:integer ?

