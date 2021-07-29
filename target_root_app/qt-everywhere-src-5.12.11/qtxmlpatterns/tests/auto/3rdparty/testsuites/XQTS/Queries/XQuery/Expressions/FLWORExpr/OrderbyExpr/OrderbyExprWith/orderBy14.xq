declare default element namespace "http://www.w3.org/XQueryTestOrderBy";
(: name : orderBy14 :)
(: description : Evaluation of "order by" clause with the "order by" clause of a FLWR expression set to "$x is $x ", where $x is a set of Strings and the ordering mode set to ascending :)

(: insert-start :)
declare variable $input-context1 external;
(: insert-end :)

<results> {
for $x in $input-context1/DataValues/Strings/orderData
 order by $x is $x ascending return $x is $x
}
</results>
