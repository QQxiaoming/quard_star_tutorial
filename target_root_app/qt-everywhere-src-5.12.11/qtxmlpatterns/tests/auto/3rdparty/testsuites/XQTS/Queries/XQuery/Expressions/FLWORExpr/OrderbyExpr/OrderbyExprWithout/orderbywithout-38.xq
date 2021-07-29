(: name : orderbywithout-38 :)
(: description : Evaluation of "order by" clause with the "order by" clause of a FLWOR expression set to "xs:decimal($x) ", where $x is a set of small negative numbers.:)

(: insert-start :)
declare variable $input-context1 external;
(: insert-end :)

<results> {
for $x in (-0.000000000000000001,-0.00000000000000001,-0.0000000000000001,-0.000000000000001,-0.00000000000001,-0.0000000000001,-0.000000000001,-0.00000000001,-0.0000000001,-0.000000001,-0.00000001,-0.0000001,-0.000001,-0.00001,-0.0001,-0.001,-0.01,-0.0,-0.1)
 order by xs:decimal($x) return xs:decimal($x)
}
</results>
