(: name : orderbylocal-28 :)
(: description : Evaluation of "order by" clause with the "order by" clause of a FLWR expression set to "xs:integer($x) ", where $x is a set of negative numbers and the ordering mode set to descending :)
(: Use a locally defined sequence. :)

(: insert-start :)
declare variable $input-context1 external;
(: insert-end :)

<results> {
for $x in (-100000000000000000,-10000000000000000,-1000000000000000,-100000000000000,-10000000000000,-1000000000000,-100000000000,-10000000000,-1000000000,-100000000,-10000000,-1000000,-100000,-10000,-1000,-100,-10,-1,-0)
 order by xs:integer($x) descending return xs:integer($x)
}
</results>
