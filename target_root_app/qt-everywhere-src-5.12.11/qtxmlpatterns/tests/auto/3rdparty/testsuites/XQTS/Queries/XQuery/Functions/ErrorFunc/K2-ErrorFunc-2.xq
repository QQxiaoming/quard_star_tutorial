(:*******************************************************:)
(: Test: K2-ErrorFunc-2                                  :)
(: Written by: Frans Englich                             :)
(: Date: 2007-11-22T11:31:21+01:00                       :)
(: Purpose: Using a QName with namespace 'none' as identifier. :)
(:*******************************************************:)
declare default element namespace "";
        fn:error(xs:QName("onlyAnNCName"))