(: name : orderbywithout-7 :)
(: description : Evaluation of "order by" clause with the "order by" clause of a FLWOR expression set to "$x is $x ", where $x is a set of nodes with strings as content. :)

(: insert-start :)
declare variable $input-context1 external;
(: insert-end :)

<results> {
for $x in (<a>A String</a>,<a>B String</a>,<a>C String</a>,<a>D String</a>,<a>E String</a>,<a>F String</a>,<a>G String</a>,<a>H String</a>,<a>I String</a>,
<a>J String</a>,<a>K String</a>,<a>L String</a>,<a>M String</a>,<a>N String</a>,<a>O String</a>,<a>P String</a>,<a>R String</a>,<a>S String</a>,<a>T String</a>, 
<a>U String</a>,<a>V String</a>,<a>W String</a>,<a>X String</a>,<a>Y String</a>,<a>Z String</a>)
 order by $x is $x return $x is $x
}
</results>
