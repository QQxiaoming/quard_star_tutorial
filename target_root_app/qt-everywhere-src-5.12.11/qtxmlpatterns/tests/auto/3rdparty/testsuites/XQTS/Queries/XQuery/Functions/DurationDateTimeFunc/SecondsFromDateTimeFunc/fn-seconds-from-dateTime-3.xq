(:*******************************************************:)
(:Test: seconds-from-dateTime-3                          :)
(:Written By: Carmelo Montanez                           :)
(:Date: June 6, 2005                                     :)
(:Purpose: Evaluates The "seconds-from-dateTime" function:)
(:involving a "numeric-less-than" operation (lt operator):)
(:*******************************************************:)

fn:seconds-from-dateTime(xs:dateTime("1999-12-31T21:20:20-05:00")) lt fn:seconds-from-dateTime(xs:dateTime("1999-12-31T21:20:20-05:00"))