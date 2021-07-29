(:*******************************************************:)
(: Test: K2-SeqReverseFunc-2                             :)
(: Written by: Frans Englich                             :)
(: Date: 2007-11-22T11:31:21+01:00                       :)
(: Purpose: Apply a confusing amount of ordered/unordered{} expressions. :)
(:*******************************************************:)
declare variable $myVar := unordered{ordered{unordered{fn:reverse((<a/>, <b/>))}}};
deep-equal($myVar, (<a/>, <b/>))
    or 
deep-equal($myVar, (<b/>, <a/>))