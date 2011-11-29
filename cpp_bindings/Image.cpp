#include "Image.h"
#include "Type.h"
#include "Util.h"
#include "Uniform.h"
#include <assert.h>

namespace FImage {
    struct DynImage::Contents {
        Contents(const Type &t, uint32_t a);
        Contents(const Type &t, uint32_t a, uint32_t b);
        Contents(const Type &t, uint32_t a, uint32_t b, uint32_t c);
        Contents(const Type &t, uint32_t a, uint32_t b, uint32_t c, uint32_t d);
        
        void allocate(size_t bytes);
        
        Type type;
        std::vector<uint32_t> size, stride;
        const std::string name;
        unsigned char *data;
        std::vector<unsigned char> buffer;
    };

    DynImage::Contents::Contents(const Type &t, uint32_t a) : 
        type(t), size{a}, stride{1}, name(uniqueName('i')) {
        allocate(a * (t.bits/8));
    }
    
    DynImage::Contents::Contents(const Type &t, uint32_t a, uint32_t b) : 
        type(t), size{a, b}, stride{1, a}, name(uniqueName('i')) {
        allocate(a * b * (t.bits/8));
    }
    
    DynImage::Contents::Contents(const Type &t, uint32_t a, uint32_t b, uint32_t c) : 
        type(t), size{a, b, c}, stride{1, a, a*b}, name(uniqueName('i')) {
        allocate(a * b * c * (t.bits/8));
    }

    DynImage::Contents::Contents(const Type &t, uint32_t a, uint32_t b, uint32_t c, uint32_t d) : 
        type(t), size{a, b, c, d}, stride{1, a, a*b, a*b*c}, name(uniqueName('i')) {
        allocate(a * b * c * d * (t.bits/8));
    }

    void DynImage::Contents::allocate(size_t bytes) {
        buffer.resize(bytes+16);
        data = &(buffer[0]);
        unsigned char offset = ((size_t)data) & 0xf;
        if (offset) {
            data += 16 - offset;
        }
    }
    
    DynImage::DynImage(const Type &t, uint32_t a) : contents(new Contents(t, a)) {}

    DynImage::DynImage(const Type &t, uint32_t a, uint32_t b) : contents(new Contents(t, a, b)) {}

    DynImage::DynImage(const Type &t, uint32_t a, uint32_t b, uint32_t c) : contents(new Contents(t, a, b, c)) {}

    DynImage::DynImage(const Type &t, uint32_t a, uint32_t b, uint32_t c, uint32_t d) : contents(new Contents(t, a, b, c, d)) {}

    const Type &DynImage::type() const {
        return contents->type;
    }

    uint32_t DynImage::stride(int i) const {
        return contents->stride[i];
    }

    uint32_t DynImage::size(int i) const {
        return contents->size[i];
    }

    int DynImage::dimensions() const {
        return contents->size.size();
    }

    unsigned char *DynImage::data() const {
        return contents->data;
    }

    const std::string &DynImage::name() const {
        return contents->name;
    }    

    Expr DynImage::operator()(const Expr &a) const {
        return ImageRef(*this, a*stride(0));
    }

    Expr DynImage::operator()(const Expr &a, const Expr &b) const {
        return ImageRef(*this, a*stride(0) + b*stride(1));
    }
    
    Expr DynImage::operator()(const Expr &a, const Expr &b, const Expr &c) const {
        return ImageRef(*this, a*stride(0) + b*stride(1) + c*stride(2));
    }
    
    Expr DynImage::operator()(const Expr &a, const Expr &b, const Expr &c, const Expr &d) const {
        return ImageRef(*this, a*stride(0) + b*stride(1) + c*stride(2) + d*stride(3));
    }

    struct UniformImage::Contents {
        Contents(const Type &t, int dims) :
            t(t), name(uniqueName('m')) {
            sizes.resize(dims);
        }

        Type t;
        std::unique_ptr<DynImage> image;
        std::vector<Uniform<int>> sizes;
        const std::string name;
    };

    UniformImage::UniformImage(const Type &t, int dims) : 
        contents(new Contents(t, dims)) {
    }

    void UniformImage::operator=(const DynImage &image) {
        assert(image.type() == contents->t);
        assert((size_t)image.dimensions() == contents->sizes.size());
        contents->image.reset(new DynImage(image));
        for (int i = 0; i < image.dimensions(); i++) {
            contents->sizes[i] = image.size(i);
        }
    }
         
    unsigned char *UniformImage::data() const {
        assert(contents->image);
        return contents->image->data();
    }

    bool UniformImage::operator==(const UniformImage &other) const {
        return contents == other.contents;
    }

    Expr UniformImage::operator()(const Expr &a) const {
        return UniformImageRef(*this, a);
    }

    Expr UniformImage::operator()(const Expr &a, const Expr &b) const {
        return UniformImageRef(*this, a + size(0) * b);
    }

    Expr UniformImage::operator()(const Expr &a, const Expr &b, const Expr &c) const {
        return UniformImageRef(*this, a + size(0) * (b + size(1) * c));
    }

    Expr UniformImage::operator()(const Expr &a, const Expr &b, const Expr &c, const Expr &d) const {
        return UniformImageRef(*this, a + size(0) * (b + size(1) * (c + size(2) * d)));
    }
    
    Type UniformImage::type() const {
        return contents->t;
    }

    const std::string &UniformImage::name() const {
        return contents->name;
    }
    
    int UniformImage::dimensions() const {
        return contents->sizes.size();
    }

    const Uniform<int> &UniformImage::size(int i) const {
        return contents->sizes[i];
    }
        
}