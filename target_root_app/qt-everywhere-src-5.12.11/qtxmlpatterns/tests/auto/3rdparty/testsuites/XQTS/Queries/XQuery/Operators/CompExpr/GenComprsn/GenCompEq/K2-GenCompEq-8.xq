(:*******************************************************:)
(: Test: K2-GenCompEq-8                                  :)
(: Written by: Frans Englich                             :)
(: Date: 2007-11-22T11:31:21+01:00                       :)
(: Purpose: The operator causes atomization.             :)
(:*******************************************************:)
empty(for $b in <e/>
where $b/@id="person0"
return ())