(:*******************************************************:)
(: Test: K2-Steps-10                                     :)
(: Written by: Frans Englich                             :)
(: Date: 2007-11-22T11:31:21+01:00                       :)
(: Purpose: The last step containing a mixture of nodes and atomic values. :)
(:*******************************************************:)
declare variable $myVar := <e>text</e>;
    $myVar/text()/(<e/>, (), 1, <e/>)