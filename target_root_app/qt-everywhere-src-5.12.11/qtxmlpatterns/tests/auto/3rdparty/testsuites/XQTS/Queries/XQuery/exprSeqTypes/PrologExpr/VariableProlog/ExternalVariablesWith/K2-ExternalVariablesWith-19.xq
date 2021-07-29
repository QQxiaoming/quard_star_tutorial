(:*******************************************************:)
(: Test: K2-ExternalVariablesWith-19                     :)
(: Written by: Frans Englich                             :)
(: Date: 2007-11-22T11:31:21+01:00                       :)
(: Purpose: variable declarations doesn't cause URI promotion conversion. :)
(:*******************************************************:)
declare variable $i as xs:string := xs:anyURI("http://www.example.com/") ; $i