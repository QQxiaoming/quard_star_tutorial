(:*******************************************************:)
(: Test: K2-DirectConOther-1                             :)
(: Written by: Frans Englich                             :)
(: Date: 2007-11-22T11:31:21+01:00                       :)
(: Purpose: Test that processing-instruction data does not receive any special treatment. :)
(:*******************************************************:)
string(<?target ""''content&amp;amp;ss&amp;#00; &amp;#x2014;?>)
                eq """""''content&amp;amp;amp;ss&amp;amp;#00; &amp;amp;#x2014;"