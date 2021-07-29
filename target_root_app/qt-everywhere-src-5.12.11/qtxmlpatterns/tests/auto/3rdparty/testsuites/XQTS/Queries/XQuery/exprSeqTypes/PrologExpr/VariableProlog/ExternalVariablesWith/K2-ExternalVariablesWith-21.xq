(:*******************************************************:)
(: Test: K2-ExternalVariablesWith-21                     :)
(: Written by: Frans Englich                             :)
(: Date: 2007-11-22T11:31:21+01:00                       :)
(: Purpose: The name of an external variable clashing with the name of a variable declared in the query(reversed order). :)
(:*******************************************************:)
declare variable $input-context1 external;
declare variable $input-context1 := 1;
1