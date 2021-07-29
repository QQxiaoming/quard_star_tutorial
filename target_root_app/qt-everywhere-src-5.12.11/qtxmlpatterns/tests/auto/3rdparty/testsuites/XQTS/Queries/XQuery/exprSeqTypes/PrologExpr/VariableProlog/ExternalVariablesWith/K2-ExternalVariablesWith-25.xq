(:*******************************************************:)
(: Test: K2-ExternalVariablesWith-25                     :)
(: Written by: Frans Englich                             :)
(: Date: 2007-11-22T11:31:21+01:00                       :)
(: Purpose: '*' is not allowed for the atomic type in element(). :)
(:*******************************************************:)
declare variable $v as element(*, xs:untyped*)+ := <e/>; 1