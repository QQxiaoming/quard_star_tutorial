(:*******************************************************:)
(: Test: K2-NameTest-48                                  :)
(: Written by: Frans Englich                             :)
(: Date: 2007-11-22T11:31:21+01:00                       :)
(: Purpose: Use 'xmlns' as an attribute name test.       :)
(:*******************************************************:)
declare variable $i := <e xmlns="http://example.com/"/>;
empty($i/@xmlns)