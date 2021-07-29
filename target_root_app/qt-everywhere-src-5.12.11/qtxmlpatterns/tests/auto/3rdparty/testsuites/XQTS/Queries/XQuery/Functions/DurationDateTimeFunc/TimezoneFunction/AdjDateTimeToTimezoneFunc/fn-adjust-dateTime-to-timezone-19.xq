(:*******************************************************:)
(:Test: adjust-dateTime-to-timezone-19                   :)
(:Written By: Carmelo Montanez                           :)
(:Date: August 10, 2005                                  :)
(:Test Description: Evaluates The "adjust-dateTime-to-timezone" function   :)
(:where an xs:dateTime value is subtracted.              :)
(:*******************************************************:)

fn:adjust-dateTime-to-timezone(xs:dateTime("2002-03-07T10:00:00-04:00")) - xs:dateTime("2001-03-07T10:00:00-05:00")