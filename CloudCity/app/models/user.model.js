const validator = require("validator");
module.exports = mongoose => {
  var schema = mongoose.Schema(
    {
      userName: {
        type: String,
        required: true,
        trim: true,
      },
      password: {
        type: String,
        required: true,
        trim: true,
      },
      email: {
        type: String,
        required: true,
        trim: true,
        validate(value) {
          if (!validator.isEmail(value)) {
            throw new Error("Invalid email !!");
          }
        },
      },
      phone: {
        type: Number,
        required: true,
        trim: true,
      },
    },
    { timestamps: true }
  );

  schema.method("toJSON", function() {
    const { __v, _id, ...object } = this.toObject();
    object.id = _id;
    return object;
  });

  const Users = mongoose.model("user", schema);
  return Users;
};
