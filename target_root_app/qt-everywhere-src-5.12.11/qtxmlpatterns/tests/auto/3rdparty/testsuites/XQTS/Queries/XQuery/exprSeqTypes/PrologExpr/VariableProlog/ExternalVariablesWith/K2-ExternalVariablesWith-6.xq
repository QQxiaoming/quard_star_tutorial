(:*******************************************************:)
(: Test: K2-ExternalVariablesWith-6                      :)
(: Written by: Frans Englich                             :)
(: Date: 2007-11-22T11:31:21+01:00                       :)
(: Purpose: Two external variables with the same name where both have a type declaration which are different. :)
(:*******************************************************:)
declare variable $input-context as xs:string* external;
declare variable $input-context as item()*external;
1