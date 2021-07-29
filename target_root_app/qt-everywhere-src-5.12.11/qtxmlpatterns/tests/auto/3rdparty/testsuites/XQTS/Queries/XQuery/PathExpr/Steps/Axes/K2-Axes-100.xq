(:*******************************************************:)
(: Test: K2-Axes-100                                     :)
(: Written by: Frans Englich                             :)
(: Date: 2007-11-22T11:31:21+01:00                       :)
(: Purpose: Do a peculiar combination between axis self and last(). :)
(:*******************************************************:)
empty(<a>
    <b/>
    <c/>
</a>[self::b][last()])