(:*******************************************************:)
(: Test: K2-NameTest-56                                  :)
(: Written by: Frans Englich                             :)
(: Date: 2007-11-22T11:31:21+01:00                       :)
(: Purpose: A '*:name' as operand to 'eq'.               :)
(:*******************************************************:)
declare variable $a := <e><a/><b/><c/></e>;
		  <a>{$a/*:ncname eq 1}</a>