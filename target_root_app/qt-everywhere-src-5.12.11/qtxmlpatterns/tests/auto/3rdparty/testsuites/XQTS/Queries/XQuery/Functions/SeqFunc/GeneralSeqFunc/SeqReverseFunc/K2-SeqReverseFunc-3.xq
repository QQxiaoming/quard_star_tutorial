(:*******************************************************:)
(: Test: K2-SeqReverseFunc-3                             :)
(: Written by: Frans Englich                             :)
(: Date: 2007-11-22T11:31:21+01:00                       :)
(: Purpose: Apply fn:unordered() to fn:reverse().        :)
(:*******************************************************:)
declare variable $myVar := unordered(fn:reverse((<a/>, <b/>)));
deep-equal($myVar, (<a/>, <b/>))
    or 
deep-equal($myVar, (<b/>, <a/>))