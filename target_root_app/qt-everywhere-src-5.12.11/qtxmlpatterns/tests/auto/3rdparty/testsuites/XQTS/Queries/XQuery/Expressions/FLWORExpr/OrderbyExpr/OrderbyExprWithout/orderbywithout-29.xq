(: name : orderbywithout-29 :)
(: description : Evaluation of "order by" clause with the "order by" clause of a FLWOR expression set to "$x is $x ", where $x is a set nodes with small positive numbers as content. :)

(: insert-start :)
declare variable $input-context1 external;
(: insert-end :)

<results> {
for $x in (<orderData>0.000000000000000001</orderData>,<orderData>0.00000000000000001</orderData>,<orderData>0.0000000000000001</orderData>,<orderData>0.000000000000001</orderData>,
<orderData>0.00000000000001</orderData>,<orderData>0.0000000000001</orderData>,<orderData>0.000000000001</orderData>,<orderData>0.00000000001</orderData>,<orderData>0.0000000001</orderData>,
<orderData>0.000000001</orderData>,<orderData>0.00000001</orderData>,<orderData>0.0000001</orderData>,<orderData>0.000001</orderData>,<orderData>0.00001</orderData>,
<orderData>0.0001</orderData>,<orderData>0.001</orderData>,<orderData>0.01</orderData>,<orderData>0.1</orderData>,<orderData>0.0</orderData>)
 order by $x is $x return $x is $x
}
</results>
