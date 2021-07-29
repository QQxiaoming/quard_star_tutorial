(:*******************************************************:)
(:Test: adjust-dateTime-to-timezone-23                   :)
(:Written By: Carmelo Montanez                           :)
(:Date: August 10, 2005                                  :)
(:Test Description: Evaluates The "adjust-dateTime-to-timezone" function   :)
(:with value of $timezone greater than PT14H.  Should raise error.:)
(:*******************************************************:)

fn:adjust-dateTime-to-timezone(xs:dateTime("2002-03-07T10:00:00-04:00"),xs:dayTimeDuration("PT15H"))