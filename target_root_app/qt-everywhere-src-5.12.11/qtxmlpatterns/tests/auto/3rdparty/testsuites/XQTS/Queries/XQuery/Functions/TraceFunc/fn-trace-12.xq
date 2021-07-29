(: Name: fn-trace-12 :)
(: Description: Simple call of "fn:trace" function used numbers manipulation queried from an xml file and the entire query is given as argument to the function. :)

(: insert-start :)
declare variable $input-context1 external;
(: insert-end :)

fn:trace((for $var in ($input-context1/works//hours) return $var + $var) ,"The Value of the given expression is: ")