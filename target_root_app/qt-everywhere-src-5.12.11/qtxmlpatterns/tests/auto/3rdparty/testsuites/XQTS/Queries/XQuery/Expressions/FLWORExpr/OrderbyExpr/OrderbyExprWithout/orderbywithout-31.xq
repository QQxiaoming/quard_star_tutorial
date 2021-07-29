(: name : orderbywithout-31 :)
(: description : Evaluation of "order by" clause with the "order by" clause of a FLWOR expression set to "xs:decimal($x) ", where $x is a set of small positive numbers.:)

(: insert-start :)
declare variable $input-context1 external;
(: insert-end :)

<results> {
for $x in (0.000000000000000001,0.00000000000000001,0.0000000000000001,0.000000000000001,0.00000000000001,0.0000000000001,0.000000000001,0.00000000001,0.0000000001,0.000000001,0.00000001,0.0000001,0.000001,0.00001,0.0001,0.001,0.01,0.1,0.0)
 order by xs:decimal($x) return xs:decimal($x)
}
</results>
