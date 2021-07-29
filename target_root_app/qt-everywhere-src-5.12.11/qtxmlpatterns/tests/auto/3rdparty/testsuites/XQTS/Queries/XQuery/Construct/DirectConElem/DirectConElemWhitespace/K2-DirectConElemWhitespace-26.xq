(:*******************************************************:)
(: Test: K2-DirectConElemWhitespace-26                   :)
(: Written by: Frans Englich                             :)
(: Date: 2007-11-22T11:31:21+01:00                       :)
(: Purpose: Extract the string value of a couple of nested elements. :)
(:*******************************************************:)
string(<a>
    {1}
    <b>
        {1}
    </b>
</a>)