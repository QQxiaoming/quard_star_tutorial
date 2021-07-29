(: name : orderbywithout-13 :)
(: description : Evaluation of "order by" clause with the "order by" clause of a FLWOR expression set to "$x is $x ", where $x is a set of nodes with negative numbers as content. :)

(: insert-start :)
declare variable $input-context1 external;
(: insert-end :)

<results> {
for $x in (<orderData>-100000000000000000</orderData>,<orderData>-10000000000000000</orderData>,<orderData>-1000000000000000</orderData>,<orderData>-100000000000000</orderData>,
<orderData>-10000000000000</orderData>,<orderData>-1000000000000</orderData>,<orderData>-100000000000</orderData>,<orderData>-10000000000</orderData>,<orderData>-1000000000</orderData>,
<orderData>-100000000</orderData>,<orderData>-10000000</orderData>,<orderData>-1000000</orderData>,<orderData>-100000</orderData>,<orderData>-10000</orderData>,<orderData>-1000</orderData>,
<orderData>-100</orderData>,<orderData>-10</orderData>,<orderData>-1</orderData>,<orderData>-0</orderData>)
 order by $x is $x return $x is $x
}
</results>
