(:*******************************************************:)
(: Test: K2-FunctionProlog-9                             :)
(: Written by: Frans Englich                             :)
(: Date: 2007-10-03T14:53:33+01:00                       :)
(: Purpose: A recursive function multiplying a sequence. :)
(:*******************************************************:)
declare function local:multiplySequence($input as xs:integer*) as xs:integer
{
    if      (empty($input))
    then    1
    else    exactly-one($input[1]) * local:multiplySequence($input[position() != 1])
};
local:multiplySequence((1, 2, 3, 4, 5))
