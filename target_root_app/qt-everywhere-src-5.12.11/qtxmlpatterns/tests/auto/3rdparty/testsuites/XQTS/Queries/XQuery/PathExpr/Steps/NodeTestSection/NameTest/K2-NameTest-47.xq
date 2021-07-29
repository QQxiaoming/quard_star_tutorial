(:*******************************************************:)
(: Test: K2-NameTest-47                                  :)
(: Written by: Frans Englich                             :)
(: Date: 2007-11-22T11:31:21+01:00                       :)
(: Purpose: Use 'xmlns' as an element name test.         :)
(:*******************************************************:)
declare variable $i := <e > <xmlns/> <xmlns/> <xmlns/> <xmlns/> </e>;
$i/xmlns