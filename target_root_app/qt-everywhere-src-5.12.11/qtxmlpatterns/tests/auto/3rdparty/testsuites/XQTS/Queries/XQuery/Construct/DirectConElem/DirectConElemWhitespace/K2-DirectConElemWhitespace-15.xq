(:*******************************************************:)
(: Test: K2-DirectConElemWhitespace-15                   :)
(: Written by: Frans Englich                             :)
(: Date: 2007-11-22T11:31:21+01:00                       :)
(: Purpose: Ensure only one text node is constructed for contiguous data. :)
(:*******************************************************:)

        count(<elem>str{""}asdas{"asd", "asd", "''", ""}{''}asd{''}{''}</elem>/text())