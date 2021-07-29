(: name : orderbylocal-44 :)
(: description : Evaluation of "order by" clause with the "order by" clause of a FLWR expression set to "$x is $x ", where $x is a set of small positive numbers and the ordering mode set to descending :)

(: insert-start :)
declare variable $input-context1 external;
(: insert-end :)

<results>{ for $x in (<a>0.000000000000000001</a>,<a>0.00000000000000001</a>,<a>0.0000000000000001</a>,<a>0.000000000000001</a>,<a>0.00000000000001</a>,<a>0.0000000000001</a>,<a>0.000000000001</a>,<a>0.00000000001</a>,<a>0.0000000001</a>,<a>0.000000001</a>,<a>0.00000001</a>,<a>0.0000001</a>,<a>0.000001</a>,<a>0.00001</a>,<a>0.0001</a>,<a>0.001</a>,<a>0.01</a>,<a>0.1</a>,<a>0.0</a>)
order by $x is $x descending return $x is $x }</results>