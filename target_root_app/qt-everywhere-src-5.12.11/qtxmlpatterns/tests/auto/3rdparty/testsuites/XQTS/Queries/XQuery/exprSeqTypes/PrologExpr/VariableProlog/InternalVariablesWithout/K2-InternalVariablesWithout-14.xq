(:*******************************************************:)
(: Test: K2-InternalVariablesWithout-14                  :)
(: Written by: Frans Englich                             :)
(: Date: 2007-11-22T11:31:21+01:00                       :)
(: Purpose: A focus doesn't propagate through variable references. :)
(:*******************************************************:)
declare variable $myVar := <e>{nametest}</e>;
<e/>/$myVar