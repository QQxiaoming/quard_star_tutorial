(:*******************************************************:)
(: Test: K2-DirectConOther-41                            :)
(: Written by: Frans Englich                             :)
(: Date: 2007-11-22T11:31:21+01:00                       :)
(: Purpose: Ensure comment content doesn't receive special interpretation. :)
(:*******************************************************:)
string(<!-- content&amp;amp;ss&amp;amp;#00; &amp;#x2014;-->) eq " content&amp;amp;amp;ss&amp;amp;amp;#00; &amp;amp;#x2014;"