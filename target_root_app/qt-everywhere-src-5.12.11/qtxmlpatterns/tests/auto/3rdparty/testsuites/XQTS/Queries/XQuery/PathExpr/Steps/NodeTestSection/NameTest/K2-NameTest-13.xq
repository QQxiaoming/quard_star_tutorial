(:*******************************************************:)
(: Test: K2-NameTest-13                                  :)
(: Written by: Frans Englich                             :)
(: Date: 2007-11-22T11:31:21+01:00                       :)
(: Purpose: A nametest being "xml:*".                    :)
(:*******************************************************:)
declare variable $var := <elem xml:space="preserve"/>;
string(($var/@xml:*)[1])