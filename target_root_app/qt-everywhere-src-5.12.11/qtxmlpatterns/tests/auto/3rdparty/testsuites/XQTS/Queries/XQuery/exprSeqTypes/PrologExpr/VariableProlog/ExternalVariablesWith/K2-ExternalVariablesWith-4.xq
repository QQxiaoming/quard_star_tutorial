(:*******************************************************:)
(: Test: K2-ExternalVariablesWith-4                      :)
(: Written by: Frans Englich                             :)
(: Date: 2007-11-22T11:31:21+01:00                       :)
(: Purpose: Two external variables with the same name where both have a type declaration and the variable is used. :)
(:*******************************************************:)
declare variable $input-context as item()* external ;
declare variable $input-context as item()*external ;
$input-context