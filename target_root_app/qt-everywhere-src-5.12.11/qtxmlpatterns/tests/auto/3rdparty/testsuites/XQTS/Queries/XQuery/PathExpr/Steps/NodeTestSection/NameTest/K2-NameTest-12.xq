(:*******************************************************:)
(: Test: K2-NameTest-12                                  :)
(: Written by: Frans Englich                             :)
(: Date: 2007-11-22T11:31:21+01:00                       :)
(: Purpose: A nametest being "xml:space".                :)
(:*******************************************************:)
declare variable $var := <elem xml:space="default"/>;
$var/@xml:space eq "default"