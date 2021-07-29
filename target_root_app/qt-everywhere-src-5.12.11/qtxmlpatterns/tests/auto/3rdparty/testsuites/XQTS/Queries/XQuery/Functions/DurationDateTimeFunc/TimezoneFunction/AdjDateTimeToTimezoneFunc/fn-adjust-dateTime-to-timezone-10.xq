(:*******************************************************:)
(:Test: adjust-dateTime-to-timezone-10                   :)
(:Written By: Carmelo Montanez                           :)
(:Date: August 10, 2005                                  :)
(:Test Description: Evaluates The "adjust-dateTime-to-timezone" function   :)
(:as part of an subtraction expression.                  :)
(:*******************************************************:)

fn:adjust-dateTime-to-timezone(xs:dateTime("2002-03-07T10:00:00-07:00")) - fn:adjust-dateTime-to-timezone(xs:dateTime("2002-03-07T10:00:00-07:00"))