(:*******************************************************:)
(: Test: orderBy62                                       :)
(: Written by: Benjamin NGUYEN & Bogdan BUTNARU          :)
(: Date: 2009-11-23+01:00                                :)
(:*******************************************************:)

(: insert-start :)
declare default element namespace "http://www.w3.org/XQueryTestOrderBy";
declare variable $input-context1 external;
(: insert-end :)

<results>{
for $x in $input-context1/DataValues/NegativeNumbers/orderData/xs:decimal(.)
where $x > -1000
order by $x ascending collation "http://www.w3.org/2005/xpath-functions/collation/codepoint"
return $x
}</results>
